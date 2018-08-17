#!/usr/bin/python
#This handles MIME types and file associations
#Files have this format: <mime-type>=<program>
#Program syntax: file-assoc [mime] [program]
import sys
import os

def init():
	home = os.getenv("HOME")
	path = home+"/.config/coffee/"
	
	if os.path.exists(path) == False:
		os.makedirs(path)
		
	path+="mime.list"
	if os.path.exists(path) == False:
		config_f = open(path,"w+")
		config_f.write("#This contains MIME and file associations for Coffee desktop")
		config_f.close()
		
def get_first(line):
	ln = ""
	for c in line:
		if c == "=":
			break
		else:
			ln+=c
	return ln
		
def assoc(mime,prog):
	home = os.getenv("HOME")
	path = home+"/.config/coffee/mime.list"
	
	content = str("")
	with open(path) as reader:
		for line in reader:
			if get_first(line) == mime:
				continue
			else:
				content+=line
				
	ln = mime+"="+prog
	content+=ln
	
	config_f = open(path,"w+")
	config_f.write(content)
	config_f.close()

def main():
	init()

	mime = ""
	prog = ""
	
	alen = len(sys.argv)
	index = 1
	
	while index<alen:
		if index == 1:
			mime = sys.argv[index]
		elif index == 2:
			prog = sys.argv[index]
		index+=1
		
	assoc(mime,prog)
	
main()
