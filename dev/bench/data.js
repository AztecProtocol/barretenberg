window.BENCHMARK_DATA = {
  "lastUpdate": 1686597953390,
  "repoUrl": "https://github.com/AztecProtocol/barretenberg",
  "entries": {
    "C++ Benchmark": [
      {
        "commit": {
          "author": {
            "name": "AztecProtocol",
            "username": "AztecProtocol"
          },
          "committer": {
            "name": "AztecProtocol",
            "username": "AztecProtocol"
          },
          "id": "cd02df3f916a4045f186296cb59d1604eae37869",
          "message": "feat: Benchmark suite update",
          "timestamp": "2023-06-06T14:27:00Z",
          "url": "https://github.com/AztecProtocol/barretenberg/pull/508/commits/cd02df3f916a4045f186296cb59d1604eae37869"
        },
        "date": 1686597952359,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "construct_proof/sha256/10/repeats:1",
            "value": 13704451261.00003,
            "unit": "ns/iter",
            "extra": "iterations: 1\ncpu: 11582012600 ns\nthreads: 1"
          },
          {
            "name": "construct_proof/keccak/10/repeats:1",
            "value": 22920204872,
            "unit": "ns/iter",
            "extra": "iterations: 1\ncpu: 20789905800 ns\nthreads: 1"
          },
          {
            "name": "construct_proof/ecdsa_verification/10/repeats:1",
            "value": 41719878376,
            "unit": "ns/iter",
            "extra": "iterations: 1\ncpu: 39049718700 ns\nthreads: 1"
          },
          {
            "name": "construct_proof/merkle_membership/10/repeats:1",
            "value": 4639689940.999915,
            "unit": "ns/iter",
            "extra": "iterations: 1\ncpu: 2779097100.0000014 ns\nthreads: 1"
          }
        ]
      }
    ]
  }
}