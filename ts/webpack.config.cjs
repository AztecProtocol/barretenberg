/**
 * Builds both the web and node version of the worker, and outputs it to the dest directory.
 */
const path = require('path');
const ResolveTypeScriptPlugin = require('resolve-typescript-plugin');
const CopyWebpackPlugin = require('copy-webpack-plugin');
const TsconfigPathsPlugin = require('tsconfig-paths-webpack-plugin');
const webpack = require('webpack');
const { resolve } = path;

module.exports = {
  target: 'web',
  mode: 'production',
  entry: {
    node: './src/index.node.ts',
    browser: './src/index.browser.ts',
  },
  module: {
    rules: [
      {
        test: /\.tsx?$/,
        use: [{ loader: 'ts-loader', options: { transpileOnly: true, onlyCompileBundledFiles: true } }],
      },
    ],
  },
  output: {
    path: resolve(__dirname, './dest'),
    filename: '[name].js',
  },
  plugins: [
    new webpack.DefinePlugin({ 'process.env.NODE_DEBUG': false }),
    new webpack.ProvidePlugin({ Buffer: ['buffer', 'Buffer'] }),
    new CopyWebpackPlugin({
      patterns: [
        {
          from: `../cpp/build-wasm/bin/barretenberg.wasm`,
          to: 'barretenberg.wasm',
        },
        {
          from: `../cpp/build-wasm-threads/bin/barretenberg.wasm`,
          to: 'barretenberg-threads.wasm',
        },
      ],
    }),
  ],
  resolve: {
    alias: {
      'idb-keyval': require.resolve('idb-keyval'),
      crypto: require.resolve('crypto-browserify'),
      path: require.resolve('path-browserify'),
      url: require.resolve('url/'),
    },
    fallback: {
      os: require.resolve('os-browserify/browser'),
      events: false,
    },
    plugins: [
      new ResolveTypeScriptPlugin(),
      new TsconfigPathsPlugin({ configFile: path.resolve(__dirname, "./tsconfig.json")})
    ],
  },
  optimization: {
    minimize: false,
  },
  
};
