module.exports = function (config) {
  config.set({
    frameworks: ['jasmine'],
    files: [
      './dest/browser-tests/tests.bundle.js',
      { pattern: 'dest/**/*.wasm', watched: false, included: false, served: true, nocache: false },
    ],
    reporters: ['progress'],
    port: 9876,
    colors: true,
    autoWatch: true,
    browsers: ['ChromeHeadless'],
    singleRun: false,
    concurrency: Infinity,
    mime: {
      'application/wasm': ['wasm'],
    },
  });
};
