#!/usr/bin/python
import argparse
import sys
import random

class LabelGenerator():
  def __init__(self, r, bits, out):
    assert(bits % 8 == 0)
    output = []
    for i in range(0, r):
      output.append(self.rand_numb(bits))
    write_output(output, out)

  def rand_numb(self, nbits):
    a = 0
    while a.bit_length() != nbits:
      a = random.getrandbits(nbits)
    return a

class MessageGenerator():
  def __init__(self, r, modulo, out):
    output = []
    for i in range(0, r):
      output.append(random.randint(0, modulo-1))
    write_output(output, out)
    
def write_output(output, outfile):
  if(outfile != None):
    fp = open(outfile, "w")
  else:
    fp = sys.stdout
  for i in output:
    fp.write("%x\n" % (i))
  if outfile != None:
    fp.close()
  
def main():
  parser = argparse.ArgumentParser(description='Generate random messages/labels')
  parser.add_argument('operation', metavar='<label | message>',
    help='what to generate: labels or messages')
  parser.add_argument('quantity', metavar='<quantity>',
    help='number labels/messages to generate')
  parser.add_argument('modulo', metavar='<nbits | modulo>',
    help='number of bits (for labels) or modulo (for messages)')
  parser.add_argument('--out', dest='outfile',
    help="where to save output")
    
  args = parser.parse_args()
  
  assert(args.operation == 'label' or args.operation == 'message')
  
  if args.operation == 'label':
    LabelGenerator(int(args.quantity), int(args.modulo), args.outfile)
  elif args.operation == 'message':
    MessageGenerator(int(args.quantity), int(args.modulo, 16), args.outfile)
    
  
if __name__ == "__main__":
  main()
