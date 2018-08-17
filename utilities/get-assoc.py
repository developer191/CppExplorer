#!/usr/bin/python
#get-assoc.py
#This gets the executable associated with an MIME type
import sys
import os

def get_first(line):
	ln = ""
	for c in line:
		if c == "=":
			break
		else:
			ln+=c
	return ln
	
def get_second(line):
	ln = ""
	found = False
	
	for c in line:
		if c=="=":
			found = True
		else:
			if found:
				ln+=c
				
	return ln

def find_assoc(mime):
	home = os.getenv("HOME")
	path = home+"/.config/coffee/mime.list"
	
	if os.path.exists(path) == False:
		print("0x1")
		
	with open(path) as reader:
		for line in reader:
			if get_first(line) == mime:
				print(get_second(line))
				exit(0)

def main():
	mime = ""
	
	slen = len(sys.argv)
	if slen == 1:
		print("0x1")
		
	arg = sys.argv[1]
	mime = str(arg)
	
	find_assoc(mime)
	
main()
