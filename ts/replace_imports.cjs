const replaceInFile = require('replace-in-file');
const path = require('path');
const dynamic_imports = require("./dynamic_imports.cjs")

const buildTarget = process.env.BUILD_TARGET;

async function replaceImports() {
  try {
    dynamic_imports.forEach(async item => {
      await replaceInFile({
        files: path.resolve(__dirname, `dest/${buildTarget}/${item}/*`),
        from: new RegExp(`'dynamic\\/${item}';`, 'g'),
        to: `'./${buildTarget}/index.js';`,
      });
    });    

    // hack to allow for shared .wasm files between build targets
    await replaceInFile({
      files: path.resolve(__dirname, `dest/${buildTarget}/barretenberg_wasm/${buildTarget}/index.js`),
      from: /new URL\(`\.\.\/\.\.\/barretenberg\.wasm`, import\.meta\.url\)/g,
      to: `new URL('../../../barretenberg.wasm', import.meta.url)`,
    });
    
  } catch (error) {
    console.error('Error occurred:', error);
  }
}

replaceImports();
