import os
import subprocess

name_to_path = []
code_to_status = {}
code_to_status[0] = "OK"
code_to_status[254] = "NOT_IMPLEMENTED"
code_to_status[255] = "IMPLEMENTED BUT INCORRECT"

def add_test_cases(dir_path):
    for root, dirs, files in os.walk(dir_path):
        for name in files:
            if not name.endswith("cpp"):
                name_to_path.append((name, os.path.join(root, name)))

def execute_test_case(path):
    p = subprocess.Popen(path, stdout=subprocess.PIPE, shell=True)
    (out, err) = p.communicate()
    code = p.wait()
    return code, out, err

add_test_cases('./mandatory/events')
add_test_cases('./mandatory/inquiry_functions')
add_test_cases('./optional')


for test, path in name_to_path:
    print('Test' + test);
    code, out, err = execute_test_case(path)
    print("Test: {} ".format(test).ljust(50)  + "    Status: [%s]" % (code_to_status[code]))
