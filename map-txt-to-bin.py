import sys

if __name__ == "__main__":
    in_file = sys.argv[1]
    out_file = sys.argv[2]
    map_data = []
    with open(in_file, "r") as f:
        l = f.readline()
        while l:
            map_data += l.split(" ")
            l = f.readline()
    map_data = [int(t) for t in map_data]
    
    with open(out_file, "wb") as f:
        f.write(bytes(map_data))