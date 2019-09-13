import os
import sys
import json


def restore_ttl():
    """ restores old ttl from json """
    if not os.path.exists('ttl.json'):
        print("please set a new ttl first")
        print_help()
        return

    try:
        with open('ttl.json', 'r') as ttl_file:
            set_ttl(int(json.load(ttl_file)['old_ttl']))
    except FileNotFoundError:
        print("'ttl.json' doesn't exist")
    except ValueError:
        print("'ttl.json' is incorrect")
    else:
        os.remove("ttl.json")


def get_old_ttl() -> int:
    ttl = 0
    current_directory = os.getcwd()

    os.chdir('/')
    with open('proc/sys/net/ipv4/ip_default_ttl', 'r') as ttl_file:
        ttl = int(ttl_file.read().strip())
    os.chdir(current_directory)

    return ttl


def set_ttl(ttl: int):
    """
    sets new ttl
        :param ttl:int: ttl to set
    """
    current_directory = os.getcwd()

    # save old ttl
    if not os.path.exists('ttl.json'):
        with open('ttl.json', 'w') as save_file:
            json.dump({"old_ttl": get_old_ttl()}, save_file)

    # set new ttl
    os.chdir('/')
    with open('proc/sys/net/ipv4/ip_default_ttl', 'w') as ttl_file:
        ttl_file.write(str(ttl))
    os.chdir(current_directory)


def print_help():
    print("setttl.py utility sets time to live of packages")
    print("usage:\npython setttl.py {restore | help | <int> | old}")
    print("'restore' restores previous ttl")
    print("'help' prints that help")
    print("'<int>' sets ttl as <int>")
    print("'old' prints current ttl")


def main():
    if len(sys.argv) is not 2:
        print_help()
    elif sys.argv[1] == 'restore':
        restore_ttl()
    elif sys.argv[1] == 'help':
        print_help()
    elif sys.argv[1] == 'old':
        print(get_old_ttl())
    else:
        try:
            new_ttl = int(sys.argv[1])

            if new_ttl <= 0:
                raise ValueError("ttl >= 1")

            set_ttl(new_ttl)
        except ValueError:
            print("incorrect input " + sys.argv[1])
            print_help()


if __name__ == "__main__":
    main()
