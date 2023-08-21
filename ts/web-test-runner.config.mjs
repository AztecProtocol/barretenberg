import { esbuildPlugin } from '@web/dev-server-esbuild';
import { playwrightLauncher } from '@web/test-runner-playwright';
import { fromRollup } from '@web/dev-server-rollup';
import rollupCommonjs from '@rollup/plugin-commonjs';

const commonjs = fromRollup(rollupCommonjs);

export default {
  browsers: [playwrightLauncher({ product: 'chromium', launchOptions: { headless: false, devtools: true } })],
  plugins: [
    esbuildPlugin({
      ts: true,
    }),
    commonjs({
      include: ['**/node_modules/debug/**/*'],
    }),
  ],
  files: ['dest/browser/**/*.browser.test.js'],
  nodeResolve: { browser: true },
  testFramework: {
    config: {
      ui: 'bdd',
      timeout: 40000,
    },
  },
};
