import os
import subprocess

dot ="."

name_to_path = []
code_to_status = {}
code_to_status[0] = "OK"
code_to_status[252] = "FATAL"
code_to_status[253] = "OMPT SHUTDOWN FAILED TO PREEMPT EXIT"
code_to_status[254] = "NOT_IMPLEMENTED"
code_to_status[255] = "IMPLEMENTED BUT INCORRECT"

def add_test_cases(dir_path):
    for root, dirs, files in os.walk(dir_path):
        for name in files:
            if name.find(dot) == -1 :
                name_to_path.append((name, os.path.join(root, name)))

def execute_test_case(path):
    p = subprocess.Popen(path, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, shell=True, universal_newlines=True)
    out = p.communicate()[0]
    code = p.wait()
    return code, out

add_test_cases('mandatory/init')
add_test_cases('mandatory/events')
add_test_cases('mandatory/inquiry_functions')
add_test_cases('optional')


try:
  for test, path in name_to_path:
    print('Running test ' + test + ' ...')
    code, out = execute_test_case(path)
    if len(out) != 0:
       print out,
    print("Result: " + test.ljust(50)  + "Status: [%s]\n" % (code_to_status[code]))
except:
    print "\nRegression tests were interrupted with a signal."
