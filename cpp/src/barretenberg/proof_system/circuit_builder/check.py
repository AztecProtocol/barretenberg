from z3 import BitVec, Solver, And, Or, Int, sat
import json
import sys
import time

p = 0x30644e72e131a029b85045b68181585d2833e84879b9709143e1f593f0000001

def add_gates(gates, vars):
    constrs = []
    for sel_wit in gates:
        #sel_wit = [x if x < p // 2 else x - p for x in sel_wit] # % p some time
        q_m, q_1, q_2, q_3, q_c, w_l, w_r, w_o = sel_wit
        eq = 0
        if q_m != 0:
            eq += q_m * vars[w_l] * vars[w_r] #% p
        if q_1 != 0:
            eq += q_1 * vars[w_l] # % p
        if q_2 != 0:
            eq += q_2 * vars[w_r] #% p
        if q_3 != 0:
            eq += q_3 * vars[w_o] #% p
        if q_c != 0:
            eq += q_c
        #print(eq)
        constrs.append(eq % p ==0)
    return constrs

fname = sys.argv[1]

circuit_info = eval(open(fname).read()) # bruh #2

variables = circuit_info['variables']
public_inps = circuit_info['public_inps']
vars_of_interest = circuit_info['vars_of_interest']
gates = circuit_info['gates']

num_vars = len(variables)
num_public_vars = len(public_inps)

s = Solver()
#vars = [BitVec(f"var_{i}", 32) for i in range(len(circuit_info["variables"]))]
vars = [Int(vars_of_interest[i]) if i in vars_of_interest else Int(f"var_{i}") for i in range(num_vars)]

for var in vars:
    s.add(And(var >= 0, var <= p - 1))

s.add(vars[0] == 0)
s.add(vars[1] == 1)
for i in public_inps:
    s.add(vars[i] == variables[i])
    #print(variables[i])

inputs = [vars[i] for i in range(len(vars)) if i in vars_of_interest]
print(inputs)
#print(vars_of_interest)

def poly(inputs):
    coeffs = inputs[:-2]
    point = inputs[-2]
    result = inputs[-1]
    ev = 0
    for i in range(len(coeffs)):
        ev = (ev * point + coeffs[i]) % p
    return (ev - result) % p != 0
    #return ev == result


s.add(poly(inputs))
s.add(And(add_gates(gates, vars)))
start = time.time()
res = s.check()
end = time.time()
print(f"Time elapsed: {end - start}, gates: {len(gates)}")
print(res)

if res == sat:
    m = s.model()
    print(vars)
    print("result = ", m[inputs[-1]])
    print("point = ", m[inputs[-2]])


# deg = 1 - 0.004
# deg = 2 - 0.02
# deg = 3 - ?