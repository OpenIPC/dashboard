import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const tagName = process.argv[2];
if (!tagName) {
  console.error('Please provide the tag name as an argument.');
  process.exit(1);
}

// Remove 'v' prefix if present for the version field, as Tauri expects semver without 'v' usually,
// but it depends on how the app version is defined. 
// However, the tag name in the URL MUST have the 'v' if the release tag has it.
const version = tagName.startsWith('v') ? tagName.slice(1) : tagName;

// Define search paths
const msiDir = path.join(__dirname, '../src-tauri/target/release/bundle/msi');
const nsisDir = path.join(__dirname, '../src-tauri/target/release/bundle/nsis');

let bundleDir;
let extension;

// Check for MSI first, then NSIS
if (fs.existsSync(msiDir) && fs.readdirSync(msiDir).some(f => f.endsWith('.msi'))) {
    bundleDir = msiDir;
    extension = '.msi';
} else if (fs.existsSync(nsisDir) && fs.readdirSync(nsisDir).some(f => f.endsWith('.exe'))) {
    bundleDir = nsisDir;
    extension = '.exe';
} else {
    console.error(`Could not find valid bundle directory. Checked:\n - ${msiDir}\n - ${nsisDir}`);
    process.exit(1);
}

const files = fs.readdirSync(bundleDir);

// Filter for files matching the current version to avoid picking up old artifacts
const installerFile = files.find(f => f.endsWith(extension) && f.includes(version));
const sigFile = files.find(f => f.endsWith(`${extension}.sig`) && f.includes(version));

if (!installerFile || !sigFile) {
  console.error(`Could not find ${extension} or ${extension}.sig files in`, bundleDir);
  console.log('Files found:', files);
  console.error('HINT: If the .sig file is missing, ensure that TAURI_SIGNING_PRIVATE_KEY is correctly set in GitHub Secrets.');
  process.exit(1);
}

const signature = fs.readFileSync(path.join(bundleDir, sigFile), 'utf8').trim();

const updateData = {
  version: version,
  notes: `Update to version ${tagName}`,
  pub_date: new Date().toISOString(),
  platforms: {
    "windows-x86_64": {
      "signature": signature,
      "url": `https://github.com/OpenIPC/dashboard/releases/download/${tagName}/${installerFile}`
    }
  }
};

const outputPath = path.join(__dirname, '../latest.json');
fs.writeFileSync(outputPath, JSON.stringify(updateData, null, 2));
console.log(`latest.json generated successfully at ${outputPath}`);
console.log(JSON.stringify(updateData, null, 2));
