import sys, getopt
import wave, struct

def read_wave(file):
    left_c_str = '{ ';
    right_c_str = '{ ';
    waveFile = wave.open(file, 'r')

    length = waveFile.getnframes()
    for i in range(0, length):
        if i > 0:
            left_c_str += ", ";
            right_c_str += ", ";
        waveData = waveFile.readframes(1)
        left_data = struct.unpack("<h", waveData[0:2])
        right_data = struct.unpack("<h", waveData[2:4])
        left_c_str += hex(left_data[0]);
        right_c_str += hex(right_data[0]);
        
    left_c_str += ' }';
    right_c_str += ' }';
    return left_c_str, right_c_str, length

def main(argv):
    help_text = "read_wave.py -i <inputfile>"
    try:
      opts, args = getopt.getopt(argv, "hi:", ["ifile="])
    except getopt.GetoptError:
      print 'read_wave.py -i <inputfile>'
      sys.exit(2)
   
    for opt, arg in opts:
      if opt == '-h':
         print 'r'
         sys.exit()
      elif opt in ("-i", "--ifile"):
         inputfile = arg
         
    left_c_str, right_c_str, length = read_wave(inputfile)  
if __name__ == "__main__":
   main(sys.argv[1:])
