// netip-handler.js (Версия с каноническим MD5 в нижнем регистре)

const net = require('net');
const fs = require('fs');
const client = require('http');
const struct = require('python-struct');
const util = require('util');
const Mutex = require('async-mutex').Mutex;
const crypto = require('crypto');

const mutex = new Mutex();

const timeout = (prom, time) => {
	let timer;
	return Promise.race([
		prom,
		new Promise((_r, rej) => timer = setTimeout(() => rej(new Error('timeout')), time))
	]).finally(() => clearTimeout(timer));
};

const QCODES = { "KeepAlive": 1006 };

module.exports = class CAM_NETIP
{ 
    constructor(setup){
        this.packet_count = 0;
        this.session = 0;
        this.status = "offline";
        this.host_ip = setup.host_ip;
        this.host_port = setup.host_port;
        this.user = setup.user || 'admin';
        this.pass = setup.pass || '';
    }

    async configure(){
        const { PromiseSocket } = await import('promise-socket');
        this.packet_count = 0;
        this.session = 0;
        this._socket = new net.Socket();         
        this.promiseSocket = new PromiseSocket(this._socket);
        await this._login();
    }
                                                                         
    async _login(){
        try {
            console.log(`[NETIP DEBUG] Connecting to ${this.host_ip}:${this.host_port}`);
            await this.promiseSocket.connect({port: this.host_port, host: this.host_ip});
            console.log(`[NETIP DEBUG] Socket connected. Attempting login with lowercase MD5 hash.`);
            
            // VVV --- ГЛАВНОЕ ИЗМЕНЕНИЕ --- VVV
            const passwordHash = this.pass ? crypto.createHash('md5').update(this.pass).digest('hex') : "";
            // ^^^ --- УБРАЛИ .toUpperCase() --- ^^^

            const loginPayload = {
                "EncryptType": "MD5",
                "LoginType": "DVRIP-Web",
                "PassWord": passwordHash,
                "UserName": this.user,
            };

            console.log('[NETIP DEBUG] Sending login payload:', JSON.stringify(loginPayload));
            const response = await this._sendData(1000, loginPayload);
            console.log('[NETIP DEBUG] Login response received:', response);

            if (response.Ret !== 100) {
                 throw new Error(`Login failed with code: ${response.Ret}. Check username/password.`);
            }

            this.session = response.SessionID;
            this.aliveInterval = setInterval(this.keep_alive.bind(this), 20000);
            this.status = "online";
            console.log(`[NETIP] Login successful. Session ID: ${this.session}`);

        } catch (error) {
            console.error(`[NETIP ERROR] on ${this.host_ip}:`, error.message || error);
            this.status = "offline";
            if(this.aliveInterval) clearInterval(this.aliveInterval);
            if(this._socket) this._socket.destroy();
            throw error; 
        }
    }

    async get_system_info(){ return await this.get_info("SystemInfo", 1020); }
    async get_general_info(){ return await this.get_info("General", 1042); }
    async get_encode_info() { return await this.get_info("Encode", 1044); }

    async get_info(about, code){
        return await this._sendData(code, { "Name": about, "SessionID": this.session });
    }

    async _sendData(command, datatoSend){
        return new Promise(async (resolve, reject) => {
            await mutex.runExclusive(async () => {
                try {
                    if (this._socket.destroyed) {
                        return reject(new Error('Socket is already destroyed.'));
                    }

                    const data = JSON.stringify(datatoSend);
                    const structed = struct.pack(
                        "BB2xII2xHI", 255, 0, this.session, this.packet_count,
                        command, data.length + 2
                    );
                    const lastPart = Buffer.from([0x0A,0x00]);
                    const TIMEOUT_MS = 10000;

                    await timeout(this.promiseSocket.write(structed), TIMEOUT_MS);
                    await timeout(this.promiseSocket.write(data), TIMEOUT_MS);
                    await timeout(this.promiseSocket.write(lastPart), TIMEOUT_MS);
                    
                    const content = await timeout(this.promiseSocket.read(), TIMEOUT_MS);

                    if (!content) {
                        throw new Error("No data received from camera.");
                    }
                
                    const responseData = JSON.parse(content.subarray(20, content.length-1));
                    this.packet_count += 1;
                    resolve(responseData);
                } catch (error) {
                    console.error(`[NETIP ERROR] in _sendData on ${this.host_ip}:`, error.message || error);
                    this.status = "offline";
                    if(this.aliveInterval) clearInterval(this.aliveInterval);
                    if(this._socket) this._socket.destroy();
                    reject(error);
                }
            });
        });
    }

    async keep_alive(){
        try {
            await this._sendData(QCODES.KeepAlive, { "Name": "KeepAlive", "SessionID": this.session }); 
        } catch (e) {
            console.warn(`Keep-alive for ${this.host_ip} failed. Connection might be lost.`);
        }
    }
}