#!/bin/bash
python make_issue_transaction.py $1 account 1
python make_issue_transaction.py $1 asset 2
python make_issue_transaction.py $1 peer 3
python make_issue_transaction.py $1 domain 4
python make_issue_transaction_shell.py $1