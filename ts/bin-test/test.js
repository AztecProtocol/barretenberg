#!/usr/bin/env node

import assert from 'node:assert';
import fs from 'fs';
import { fileURLToPath } from 'url';
import path from 'node:path';
import { describe, it } from 'node:test';

import { exec } from 'node:child_process';

const executableScriptLocation = '../dest/node/main.js';
const compiledNoirProgramLocation = 'target/main.json';
const solvedWitnessLocation = 'target/witness.tr';
const executionScript = path.join(getScriptDirectory(), executableScriptLocation);

// Skips warnings, ie. experimental features
process.removeAllListeners('warning');

function getScriptDirectory() {
  const moduleURL = import.meta.url;
  const modulePath = fileURLToPath(moduleURL);
  const scriptDirectory = path.dirname(modulePath);

  return scriptDirectory
}


describe('Test Pre-Requisits', () => {

  it('dest/node/main.js exists', () => {

    const scriptDirectory = getScriptDirectory();

    const filePath = path.join(scriptDirectory, executableScriptLocation);

    const fileExists = fs.existsSync(filePath);

    assert.ok(fileExists, `File does not exist at ${filePath}, execute 'yarn build:ts:node' first.`);
  });

  it('bin-test/target/main.json exists', () => {

    const scriptDirectory = getScriptDirectory();

    const filePath = path.join(scriptDirectory, compiledNoirProgramLocation);

    const fileExists = fs.existsSync(filePath);

    assert.ok(fileExists, `File does not exist at ${filePath}, execute 'nargo compile' first.`);
  });

  it('bin-test/target/witness.tr exists', () => {

    const scriptDirectory = getScriptDirectory();

    const filePath = path.join(scriptDirectory, solvedWitnessLocation);

    const fileExists = fs.existsSync(filePath);

    assert.ok(fileExists, `File does not exist at ${filePath}, execute 'nargo execute' first.`);
  });

});

describe('Test bb subcommands', () => {

  it('get gate count using `bb gates` command', (done) => {
    
    const expectedOutputTemplate = /^gates: \d+$/;
    const executionDirectory = getScriptDirectory();
    const command = `${executionScript} gates`;

    exec(command, { cwd: executionDirectory }, function(error, stdout, stderr) {
      if (error) {
        done(error);
        return;
      }

      // Assert that the stdout matches the expected output template
      assert.match(stdout.trim(), expectedOutputTemplate);

      done();
    });
  });

  it('creates proof with `bb prove -o proof` command', (done) => {
    
    const expectedOutputTemplate = /^proof written to: proof$/;
    const executionDirectory = getScriptDirectory();
    const command = `${executionScript} prove -o proof`;

    exec(command, { cwd: executionDirectory }, function(error, stdout, stderr) {
      if (error) {
        done(error);
        return;
      }

      // Assert that the stdout matches the expected output template
      assert.match(stdout.trim(), expectedOutputTemplate);

      done();
    });
  });

  it('writes verification key with `bb write_vk -o vk` command', (done) => {
    
    const expectedOutputTemplate = /^vk written to: vk$/;
    const executionDirectory = getScriptDirectory();
    const command = `${executionScript} write_vk -o vk`;

    exec(command, { cwd: executionDirectory }, function(error, stdout, stderr) {
      if (error) {
        done(error);
        return;
      }

      // Assert that the stdout matches the expected output template
      assert.match(stdout.trim(), expectedOutputTemplate);

      done();
    });
  });

  it('verifies proof with `bb verify -k vk -p proof` command', (done) => {
    
    const expectedOutputTemplate = /^verified: true$/;
    const executionDirectory = getScriptDirectory();
    const command = `${executionScript} verify -k vk -p proof`;

    exec(command, { cwd: executionDirectory }, function(error, stdout, stderr) {
      if (error) {
        done(error);
        return;
      }

      // Assert that the stdout matches the expected output template
      assert.match(stdout.trim(), expectedOutputTemplate);

      done();
    });
  });
  
  
});
