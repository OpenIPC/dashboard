// file: scripts/set-version.js
const fs = require('fs');
const path = require('path');

const version = process.argv[2];
if (!version || (version !== 'lite' && version !== 'intellect')) {
    console.error('Error: Version must be "lite" or "intellect".');
    process.exit(1);
}

const config = { version };
const configPath = path.join(__dirname, '..', 'version-config.json');
fs.writeFileSync(configPath, JSON.stringify(config));
console.log(`Application version set to: ${version}`);