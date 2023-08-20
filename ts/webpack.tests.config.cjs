const path = require('path');
const ResolveTypeScriptPlugin = require('resolve-typescript-plugin');
const CopyWebpackPlugin = require('copy-webpack-plugin');
const TsconfigPathsPlugin = require('tsconfig-paths-webpack-plugin');
const webpack = require('webpack');
const HtmlWebpackPlugin = require('html-webpack-plugin');
const { resolve } = path;
const glob = require('glob');

// Dynamically get the list of test files
const testFiles = glob.sync('./src/**/*.browser.test.ts');

console.log(testFiles);

const buildTarget = 'browser';
const configFile = path.resolve(__dirname, `./tsconfig.${buildTarget}.json`);

module.exports = {
  mode: 'development',
  entry: testFiles,
  target: 'web',
  output: {
    path: resolve(__dirname, `./dest/${buildTarget}-tests`),
    filename: 'tests.bundle.js',
    publicPath: '/',
  },
  module: {
    rules: [
      {
        test: /\.ts?$/,
        use: [
          {
            loader: 'ts-loader',
            options: {
              transpileOnly: true,
              onlyCompileBundledFiles: true,
              configFile,
            },
          },
        ],
      },
    ],
  },
  resolve: {
    plugins: [new ResolveTypeScriptPlugin(), new TsconfigPathsPlugin({ configFile })],
    fallback: {
      path: require.resolve('path-browserify'),
    },
  },
  optimization: {
    minimize: false,
  },
  plugins: [
    new webpack.DefinePlugin({ 'process.env.NODE_DEBUG': false }),
    new webpack.ProvidePlugin({ Buffer: ['buffer', 'Buffer'] }),
    new CopyWebpackPlugin({
      patterns: [
        {
          from: `../cpp/build-wasm/bin/barretenberg.wasm`,
          to: '../barretenberg.wasm',
        },
        {
          from: `../cpp/build-wasm-threads/bin/barretenberg.wasm`,
          to: '../barretenberg-threads.wasm',
        },
      ],
    }),
    new HtmlWebpackPlugin({
      template: './src/index.html',
      inject: 'body',
    }),
  ],
  devServer: {
    contentBase: path.join(__dirname, 'dest'),
    compress: true,
    port: 8080,
  },
};
