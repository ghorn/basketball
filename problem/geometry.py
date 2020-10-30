import sympy

def solve1():
  vf = sympy.Symbol('vf')
  a = sympy.Symbol('a')
  v0 = sympy.Symbol('v0')
  t = sympy.Symbol('t')
  p0 = sympy.Symbol('p0')
  pf = 0 # sympy.Symbol('pf')

  equalities = [
    (vf, v0 + a * t),
    (pf, p0 + v0 * t + a * t * t / 2),
  ]
  system = [lhs - rhs for lhs, rhs in equalities]
    
  variables, solutions = sympy.solve(system, v0, t, set=True)
  for k_sol, sol in enumerate(solutions):
    print()
    print('solution {}:'.format(k_sol))
    for k_var, variable in enumerate(variables):
      print('  {} = {}'.format(str(variable), str(sol[k_var])))

#def solve2():
#  vf = sympy.Symbol('vf')
#  a = sympy.Symbol('a')
#  v0 = sympy.Symbol('v0')
#  t = sympy.Symbol('t')
#  p0 = sympy.Symbol('p0')
#  pf = 0
#
#  equalities = [
#    (vf, v0 + a * t),
#    (pf, p0 + v0 * t + a * t * t / 2),
#  ]
#  system = [lhs - rhs for lhs, rhs in equalities]
#    
#  variables, solutions = sympy.solve(system, v0, t, set=True)
#  for k_sol, sol in enumerate(solutions):
#    print()
#    print('solution {}:'.format(k_sol))
#    for k_var, variable in enumerate(variables):
#      print('  {} = {}'.format(str(variable), str(sol[k_var])))

def main():
  solve1()
  print('------------')
  solve2()

if __name__=='__main__':
  main()
