# -*- coding: utf-8 -*-

import sys
# gm = function_name ( dust )
# ca = command_name lower_case
# da = datamodel_name lower_case
(gm, ca, da, index) = sys.argv

cb = (ca[0]).upper() + ca[1:]
cc = ca.upper()

db = (da[0]).upper() + da[1:]
dc = da.upper()

code_in = open("template_make_issue_transaction_in.hpp")
code_out = open("temp_"+index,'w')

for line in code_in.readlines():
    line = line.replace('COMMAND_NAME_A',ca) \
        .replace('COMMAND_NAME_B',cb) \
        .replace('COMMAND_NAME_C',cc) \
        .replace('DATAMODEL_NAME_A',da) \
        .replace('DATAMODEL_NAME_B',db) \
        .replace('DATAMODEL_NAME_B',dc)
    if line.find("##CODE") != -1:
        code_code = open("create_temp_"+da+".hpp")
        line = code_code.read()
    code_out.write( line )

code_in.close()
code_out.close()