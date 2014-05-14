import sys, getopt, os, glob
import string

from read_wave import read_wave

def write_c_header(data):
    output_string = ''
    output_string += 'const int kHRTFOrientation[' + str(len(data)) + '][2] = { '
    
    add_comma = False
    for elevation, azimuth, left_c_str, right_c_str, length in data:
        if add_comma: 
            output_string += ", " 
        add_comma = True
        
        output_string += '{' + str(elevation) + ', ' + str(azimuth) + '}'
    output_string += ' };\n'

    
    output_string += 'const int kHRTF[' + str(len(data)) + '][2][' + str(length) + '] = { \n'
    add_comma = False
    for elevation, azimuth, left_c_str, right_c_str, length in data:
        if add_comma: 
            output_string += ", " 
        add_comma = True
        output_string += '{ ' + left_c_str + ',\n  ' + right_c_str + ' }\n'
    output_string += '};\n'

    return output_string;  

def dir_scan_azimuth(scan_folder, elevation, result):
    scan_str = 'H' + str(elevation) + 'e'
    
    for file in glob.glob(scan_folder + "/" + scan_str + "*.wav"):
        azimuthstrpos = string.find(file, scan_str)
        if azimuthstrpos > 0:
            azimuth = int(file[azimuthstrpos + len(scan_str):azimuthstrpos + len(scan_str) + 3])
            left_c_str, right_c_str, length = read_wave(file)  
            result.append((elevation, azimuth, left_c_str, right_c_str, length))
    
def dir_scan_elevation(scan_folder):
    scan_str = 'elev'
    result = []

    for folder, subs, files in os.walk(scan_folder):
        elevstrpos = string.find(folder, scan_str)
        if elevstrpos > 0:
            elevation = int(folder[elevstrpos + len(scan_str):])
            dir_scan_azimuth(folder, elevation, result)
            
    result.sort(key=lambda tup: tup[0] * 1000 + tup[1])
    return result

def main(argv):
    help_text = "create_kemar_hrtf_header.py -i <inputfile>"
    try:
      opts, args = getopt.getopt(argv, "hi:", ["ifile="])
    except getopt.GetoptError:
      print 'create_kemar_hrtf_header.py -i <inputfile>'
      sys.exit(2)
   
    for opt, arg in opts:
      if opt == '-h':
         print 'r'
         sys.exit()
      elif opt in ("-i", "--ifile"):
         inputfile = arg
    
    data = dir_scan_elevation(inputfile)
    
    print write_c_header(data)
    
if __name__ == "__main__":
   main(sys.argv[1:])
