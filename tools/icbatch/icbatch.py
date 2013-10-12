'''
Created on 2013-10-11

@author: kevin
'''

import os
import sys
import stat
import os.path
import configparser

# default options
iconfig_file = '_iconfig.ini'
icropper_cmd = 'ic.exe'

default_config = { \
"block_size":100, \
"crop_max_depth":4, \
"crop_max_ratio":0.6, \
"crop_min_area":1000, \
"enable_rotate":True, \
"fixed_texture_size":0, \
"force_single":False, \
"max_texture_size":2048, \
"scale":1, \
"texture_padding":1, \
"texture_suffix":"png", \
"y_axis_up":True, \
"icb_only":False, \
"icbfile_suffix":"icb", \
"xml_only":False, \
"xmlfile_suffix":"xml", \
"ignores":".svn".split(sep=","), \
"filters":".png,.bmp".split(sep=","), \
"process_all":True, \
"pack_dir":False, \
"pack_name":"", \
}

exe_filter_opts = ('filters', 'ignores', 'pack_dir', 'pack_name', 'process_all')

# config stack
current_config = None
config_stack = []
current_path = ""
current_meshes = {}
output_root_path = ""
input_root_path = ""


def to_bool(b):
    if b == '1' or b == "True" or b == "true":
        return True
    return False

def parse_opt_array(parser, section, option):
    items = []
    if parser.has_section(section):
        counter = 0
        while True:
            option_name = option + "[" + str(counter) + "]";
            if not parser.has_option(section, option_name):
                break
            items.append(parser[section][option_name])
            counter += 1
    return items

'''
    read ICropper config file
'''
def read_config():
    global current_config
    global global_counter
    global current_meshes
    
    # copy current config
    if current_config == None:
        current_config = default_config
    new_config = current_config.copy()
    
    # initial value
    current_meshes = {}
    
    # read config file
    read_config = {}
    parser = configparser.ConfigParser()
    if parser.read(os.path.join(current_path, iconfig_file), 'utf-8'):
        if parser.has_option("OPTIONS", "block_size"):
            read_config["block_size"] = int(parser["OPTIONS"]["block_size"])
        if parser.has_option("OPTIONS", "crop_max_depth"):
            read_config["crop_max_depth"] = int(parser["OPTIONS"]["crop_max_depth"])
        if parser.has_option("OPTIONS", "crop_max_ratio"):
            read_config["crop_max_ratio"] = float(parser["OPTIONS"]["crop_max_ratio"])
        if parser.has_option("OPTIONS", "crop_min_area"):
            read_config["crop_min_area"] = int(parser["OPTIONS"]["crop_min_area"])
        if parser.has_option("OPTIONS", "enable_rotate"):
            read_config["enable_rotate"] = to_bool(parser["OPTIONS"]["enable_rotate"])
        if parser.has_option("OPTIONS", "fixed_texture_size"):
            read_config["fixed_texture_size"] = int(parser["OPTIONS"]["fixed_texture_size"])
        if parser.has_option("OPTIONS", "force_single"):
            read_config["force_single"] = to_bool(parser["OPTIONS"]["force_single"])
        if parser.has_option("OPTIONS", "max_texture_size"):
            read_config["max_texture_size"] = int(parser["OPTIONS"]["max_texture_size"])
        if parser.has_option("OPTIONS", "scale"):
            read_config["scale"] = float(parser["OPTIONS"]["scale"])
        if parser.has_option("OPTIONS", "texture_padding"):
            read_config["texture_padding"] = int(parser["OPTIONS"]["texture_padding"])
        if parser.has_option("OPTIONS", "texture_suffix"):
            read_config["texture_suffix"] = parser["OPTIONS"]["texture_suffix"]
        if parser.has_option("OPTIONS", "y_axis_up"):
            read_config["y_axis_up"] = to_bool(parser["OPTIONS"]["y_axis_up"])
        if parser.has_option("OPTIONS", "icb_only"):
            read_config["icb_only"] = to_bool(parser["OPTIONS"]["icb_only"])
        if parser.has_option("OPTIONS", "icbfile_suffix"):
            read_config["icbfile_suffix"] = parser["OPTIONS"]["icbfile_suffix"]
        if parser.has_option("OPTIONS", "xml_only"):
            read_config["xml_only"] = to_bool(parser["OPTIONS"]["xml_only"])
        if parser.has_option("OPTIONS", "xmlfile_suffix"):
            read_config["xmlfile_suffix"] = parser["OPTIONS"]["xmlfile_suffix"]
        if parser.has_option("OPTIONS", "ignores"):
            read_config["ignores"] = str(parser["OPTIONS"]["ignores"]).split(sep=",")
        if parser.has_option("OPTIONS", "process_all"):
            read_config["process_all"] = to_bool(parser["OPTIONS"]["process_all"])
        if parser.has_option("OPTIONS", "pack_dir"):
            read_config["pack_dir"] = to_bool(parser["OPTIONS"]["pack_dir"])
        if parser.has_option("OPTIONS", "pack_name"):
            read_config["pack_name"] = parser["OPTIONS"]["pack_name"]
            
        if parser.has_section("OUTPUT"):
            output_files = parse_opt_array(parser, "OUTPUT", "OUT")
            for item in output_files:
                files = parse_opt_array(parser, item, "FILE")
                if files:
                    current_meshes[item] = files
    
    # update config with config file
    if read_config:
        new_config.update(read_config)
    
    return new_config

'''
    check if ignore the path
'''
def check_ignore(filename):
    for item in current_config["ignores"]:
        if item.lower() == filename.lower():
            return True
    
    return False

'''
    check if filter the file
'''
def filter_filetype(filename):
    ign = True
    for item in current_config["filters"]:
        if filename.lower().endswith(item):
            ign = False
            break
    
    return ign

'''
    push ICropper config file
'''
def push_config(config):
    global current_config
    config_stack.append(config)
    current_config = config
    #print(current_config)

'''
    pop ICropper config file
'''
def pop_config():
    current_config = config_stack.pop()
    
    
'''
    execute icropper
'''
def execute_icropper(root_inputpath, input_files, rel_path, root_outputpath, output_file):
    is_ok = True
    src_path = " -src_path=" + root_inputpath
    out_path = " -out_path=" + root_outputpath
    out_file = " -out_file=" + os.path.join(rel_path, output_file)
    src_files = " -src_files=\""
    for f in input_files:
        src_files += os.path.join(rel_path, f) + " "
    src_files.rstrip()
    src_files += "\""
    
    cmdline = icropper_cmd + src_path + src_files + out_path + out_file
    #print(cmdline)
    
    for k, v in current_config.items():
        if not k in exe_filter_opts:
            cmdline += " -" + k + "=" + str(v)
    
    pipe = os.popen(cmdline)
    out = pipe.read()
    if out:
        print("..............................................")
        print(out)
        print("..............................................")
        is_ok = False

    return is_ok

'''
    process free file
'''
def process_free_file(full_path, rel_path, file_name):
    output_file = file_name[0:file_name.rfind(".")]
    print(" --FILE: " + file_name + " ===> " + output_file + ".*", end='')
    if execute_icropper(input_root_path, [file_name], rel_path, output_root_path, output_file):
        print(" [OK]");
    else:
        print(" [FAILED]");
    
'''
    process configured files
'''
def process_configured_files(full_path, rel_path, file_name):
    global current_meshes
    ok_set = set()
    if current_meshes:
        for k, v in current_meshes.items():
            print(" --FILES: " + str(v) + " ===> " + k + ".*", end='')
            if execute_icropper(input_root_path, v, rel_path, output_root_path, k):
                print(" [OK]");
            else:
                print(" [FAILED]");
            for item in v:
                ok_set.add(item)
    
    return ok_set
    
'''
    pack directory
'''
def pack_dir(full_path, rel_path, dir_name):
    files = []
    for item in os.listdir(full_path):
        subpath = os.path.join(full_path, item)
        mode = os.stat(subpath)[stat.ST_MODE]
        if not stat.S_ISDIR(mode) and not check_ignore(item) and not filter_filetype(item):
            files.append(item)
        
    if files:
        output_name = current_config["pack_name"]
        if not output_name:
            output_name = dir_name

        print(" --PACK: " + str(files) + " ===> " + output_name + ".*", end='')
        if execute_icropper(input_root_path, files, rel_path, output_root_path, output_name):
            print(" [OK]");
        else:
            print(" [FAILED]");

'''
    visit directories
'''
def walk(full_path, rel_path, file_name):
    global current_path
    current_path = full_path
    print("*Processing: " + full_path)
    
    config = read_config()
    push_config(config)
    
    # pack directory files
    if current_config["pack_dir"]:
        pack_dir(full_path, rel_path, file_name)
    else: 
        # process specified files
        ok_set = process_configured_files(full_path, rel_path, file_name)
        
        # walk left files
        if current_config["process_all"]:
            for item in os.listdir(full_path):
                if item in ok_set:
                    continue
                subpath = os.path.join(full_path, item)
                mode = os.stat(subpath)[stat.ST_MODE]
                if not stat.S_ISDIR(mode) and not check_ignore(item) and not filter_filetype(item):
                    process_free_file(full_path, rel_path, item)
            
    # walk sub directories
    for item in os.listdir(full_path):
        subpath = os.path.join(full_path, item)
        rel_subpath = os.path.join(rel_path, item)
        mode = os.stat(subpath)[stat.ST_MODE]
        if stat.S_ISDIR(mode) and not check_ignore(item):
            walk(subpath, rel_subpath, item)
        else:
            pass
                
    pop_config()

'''
    Main Entry
'''
if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("[ERR]:must specify the input path!")
        exit(0)
    input_root_path = sys.argv[1]
    if len(sys.argv) < 3:
        output_root_path = input_root_path
    else:
        output_root_path = sys.argv[2]
    walk(input_root_path, "", "root")
    
    
