#!/usr/bin/env python
import fileinput, sys, getopt, os


if __name__ == '__main__':
  print('Reading arguments...')
  # obtengo los argumentos
  argv = sys.argv[1:]

  try:
      opts, args = getopt.getopt(argv,"hn:p:", ["name=", "path="])
  except getopt.GetoptError:
      print('build_impl.py -h -n <class_name> -p <output_path>')
      sys.exit(2)
  finally:
      pass

  filename = ''
  filepath = ''

  for opt, arg in opts:
      if opt in("-n", "--name"):
          filename = arg  
      elif opt in("-p", "--path"):
          filepath = arg
      else:
        print('build_impl.py -h -n <class_name> -p <output_path>')
        sys.exit(2)

  currfile = os.getcwd() + '\ActiveModuleImpl'

  extensions = ['.h', '.cpp']
  for e in extensions:
    try:
      f_in = currfile + e
      f_out = filepath+'\\' + filename + '\\' + filename + e
      print('Opening input file: ',f_in)
      print('Opening output file: ',f_out)
      
      # check if out_dir exists, else it creates it
      out_dir = os.path.dirname(f_out)
      if not os.path.exists(out_dir):
        try:
          os.makedirs(out_dir)
        except OSError as e:
          print('Error creating folder: ', out_dir)
          sys.exit(2)

      # open input file
      s = open(f_in).read()
      print('Replacing text "ActiveModuleImpl" with text: ', filename)
      # replace text
      s = s.replace('ActiveModuleImpl', filename)
      print('Saving changes to file: ', f_out)
      # save changes to output file
      f = open(f_out, 'w')
      f.write(s)
      f.close()
    except IOError:
      print('Error:', IOError.errno)
      sys.exit(2)

