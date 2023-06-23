const replaceInFile = require('replace-in-file');
const path = require('path');
const dynamic_imports = require("./dynamic_imports.cjs")

const buildTarget = process.env.BUILD_TARGET;

async function replaceImports() {
  try {
    dynamic_imports.forEach(async item => {
      const results = await replaceInFile({
        files: path.resolve(__dirname, `dest/${item}/*`),
        from: new RegExp(`'dynamic\\/${item}';`, 'g'),
        to: `'./${buildTarget}/index.js';`,
      });
      console.log(results.filter(r => r.hasChanged === true))
    });    
  } catch (error) {
    console.error('Error occurred:', error);
  }
}

replaceImports();
