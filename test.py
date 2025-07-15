import sc2reader
import sys

if __name__ == '__main__':
    path = sys.argv[1]
    replay = sc2reader.load_replay(path)
    for message in replay.messages:
        print(message.text)
