from z3 import BitVec, Solver, And, Or, Int
import json

circuit_info = eval(open("circuit.json").read()) # bruh #2
p = 0x30644e72e131a029b85045b68181585d2833e84879b9709143e1f593f0000001


s = Solver()

#vars = [BitVec(f"var_{i}", 32) for i in range(len(circuit_info["variables"]))]
vars = [Int(f"var_{i}") for i in range(len(circuit_info["variables"]))]

for i in circuit_info["public_inps"]:
    s.add(vars[i] == circuit_info["variables"][i])


for sel_wit in circuit_info["gates"]:
    sel_wit = [x if x < p // 2 else x - p for x in sel_wit] # bruh
    q_m, q_1, q_2, q_3, q_c, w_l, w_r, w_o = sel_wit
    s.add(q_m * vars[w_l]  * vars[w_r] + q_1 * vars[w_l] + q_2 * vars[w_r] + q_3 * vars[w_o] + q_c == 0)


print(s.check())