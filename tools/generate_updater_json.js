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

const bundleDir = path.join(__dirname, '../src-tauri/target/release/bundle/msi');

if (!fs.existsSync(bundleDir)) {
    console.error(`Bundle directory does not exist: ${bundleDir}`);
    process.exit(1);
}

const files = fs.readdirSync(bundleDir);

// Filter for files matching the current version to avoid picking up old artifacts
const msiFile = files.find(f => f.endsWith('.msi') && f.includes(version));
const sigFile = files.find(f => f.endsWith('.msi.sig') && f.includes(version));

if (!msiFile || !sigFile) {
  console.error('Could not find .msi or .msi.sig files in', bundleDir);
  console.log('Files found:', files);
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
      "url": `https://github.com/OpenIPC/dashboard/releases/download/${tagName}/${msiFile}`
    }
  }
};

const outputPath = path.join(__dirname, '../latest.json');
fs.writeFileSync(outputPath, JSON.stringify(updateData, null, 2));
console.log(`latest.json generated successfully at ${outputPath}`);
console.log(JSON.stringify(updateData, null, 2));
