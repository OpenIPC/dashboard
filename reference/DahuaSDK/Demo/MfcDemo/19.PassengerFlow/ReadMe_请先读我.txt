【Demo功能】
1、Demo介绍SDK初始化,登陆设备, 登出设备, 订阅智能事件, 接收智能事件报警, 解析智能事件信息, 停止智能事件, 人流量统计, 客流历史数据的查询
2、Demo演示的智能事件包括：滞留时长报警, 滞留人数、离开人数、进入人数报警, 区域内人数上限报警.

【注意事项】
1、编译环境为VS2005。
2、人流量统计的的“规则线”绘制需要在web端完成，若无规则线则无法触发智能事件和实时人流量统计上报。
3、如需运行demo的exe文件，请将开发包中【库文件】里的全部dll文件拷贝至该工程目录下的bin/x86release或者bin/x64release下。
4、如把库文件放入程序生成的目录中，运行有问题，请到大华官网下载最新的网络SDK版本：http://www.dahuatech.com/index.php/service/downloadlists/836.html 替换程序中的库文件

【Demo Features】
1.Demo SDK initialization,login device,logout device, subscribe to intelligent event information, receive intelligent event information, parse event information, people counting, passenger flow historical data query
2.Demo a part of intelligent event include Detention duration, detained person, the number of people entering the number , Maximum number of people in the region

【NOTE】
1.Complier for Demo is VS2005.
2.before subscribe to intelligent event or subscirbe to people counting you should draw the rule line on the web ,without it the intelligent event would not notify so thant you will not receive event.
3.If you want to run the demo exe file, please copy all dll files in the development package [Bin] to bin/x86release or bin/x64release under the project directory
4.If run the program with some problems,please go to http://www.dahuasecurity.com/download_3.html download the newest version,and replace the DLL files