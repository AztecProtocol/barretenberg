import * as bbjs from './index.js';

const originalFetch = window.fetch;

function transformURL(url) {
  if (typeof url !== 'string' || !url.startsWith('http://localhost:9876/')) {
    throw new Error('Invalid URL format');
  }
  const fileName = url.split('/').pop();
  // karma serves the .wasm files on /base/dest/browser-tests
  return `http://localhost:9876/base/dest/browser-tests/${fileName}`;
}

function mockFetch(input: RequestInfo) {
  const url = transformURL(input);
  console.log('input path:', url, input);
  return originalFetch(url);
}

(window as any).fetch = mockFetch;

describe('browser', () => {
  it('should initialize Barretenberg API', async () => {
    const api = await bbjs.newBarretenbergApiSync();
    console.log(api);
    expect(api).toBeDefined();
  }, 5e3);
});
