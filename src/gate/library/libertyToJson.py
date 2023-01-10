#!/usr/bin/env python3
# После парсинга имеем следующую структуру:
# "Название ячейки": {
#   input: "вход1, вход2",
#   output: {
#       "выход" : "логическая функция",
#       ...
#   }
# }
import json
from sys import argv
from liberty.parser import parse_liberty

script, pathToLibraryFile, pathToJsonFile = argv

def read_information_from_library(path_to_library):
  library = parse_liberty(open(path_to_library).read())

  input_pins = ""
  output_pins = {}
  cell_library = {}
  last_output_pins = {}

  for cell_group in library.get_groups('cell'):
    cell_name = (str(cell_group.args[0]))[1: -1]

    for pin_group in cell_group.get_groups('pin'):
      pin_name_attribute = str(pin_group.args[0])[1: -1]

      pin_direction_attribute = pin_group['direction']

      if pin_direction_attribute == "output":
        pin_function_attribute = str(pin_group["function"])[1: -1]
        output_pins[pin_name_attribute] = pin_function_attribute

      else:
        input_pins += pin_name_attribute + " "

    if ((output_pins != last_output_pins) and ("Q" not in output_pins)
        and ("CLK" not in input_pins) and (len(output_pins) != 0)):
      cell_library[cell_name] = {'input': input_pins,
                                'output': output_pins}
      last_output_pins = output_pins

    input_pins = ""
    output_pins = {}
    
  return cell_library

def write_to_json(cells, path_to_file):
  with open(path_to_file, 'w') as outfile:
    json.dump(cells, outfile, sort_keys=True, indent=4)

write_to_json(read_information_from_library(pathToLibraryFile), pathToJsonFile)
