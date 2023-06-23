const fs = require('fs');
const path = require('path');
const dynamic_imports = require("./dynamic_imports.cjs")

const buildTarget = process.env.BUILD_TARGET;

function generatePaths() {
  const paths = {};

  dynamic_imports.forEach(item => {
    const key = `dynamic/${item}`;
    const value = [`src/${item}/${buildTarget}/index.ts`];
    paths[key] = value;
  });

  return paths;
}

const tsconfig = {
  compilerOptions: {
    baseUrl: ".",
    target: "es2020",
    lib: ["dom", "esnext", "es2017.object"],
    module: "NodeNext",
    strict: false,
    declaration: true,
    allowSyntheticDefaultImports: true,
    esModuleInterop: true,
    downlevelIteration: true,
    inlineSourceMap: true,
    declarationMap: true,
    importHelpers: true,
    resolveJsonModule: true,
    composite: true,
    outDir: `dest/${buildTarget}`,
    rootDir: "src",
    tsBuildInfoFile: ".tsbuildinfo",
    paths: generatePaths()
  },
  include: ["src"]
};

fs.writeFileSync(path.resolve(__dirname, 'tsconfig.json'), JSON.stringify(tsconfig, null, 2));