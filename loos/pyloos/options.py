#!/usr/bin/env python3

import argparse
import sys

"""
    Parse command line options and implement common sets of options
    Implemented as a wrapper around argparse
"""
class LoosOptions:
    def __init__(self, fullhelp=None):
        self.parser = argparse.ArgumentParser()
        if fullhelp:
            self.setFullhelp(fullhelp)
        if len(sys.argv)==1:
            parser.print_help(sys.stderr)
            sys.exit(1)


    def setFullhelp(self, fullhelp=None):
        self.fullhelp = fullhelp
        self.parser.add_argument('--fullhelp',
                                 action='store_true',
                                 default=False)

    # Set up some default arguments
    def modelSelectionOptions(self):
        self.parser.add_argument('--model',
                                 help="Model file describing system contents")
        self.parser.add_argument('--sel',
                                 help='Use this selection for computation',
                                 default='all')

    def trajOptions(self):
        self.parser.add_argument('--traj',
                                 help='Filename of trajectory or trajectories',
                                 nargs='+')
        self.parser.add_argument('--skip',
                                 help='Skip frames from the trajectory start',
                                 type=int,
                                 default=0)
        self.parser.add_argument('--stride',
                                 help='Step through the trajectory by this',
                                 type=int,
                                 default=1)

    def parse_args(self):
        args = self.parser.parse_args()
        # postprocess args here; error checks, checks for needed behavior
        if args.fullhelp:
            sys.stderr.write(self.fullhelp)
            sys.exit(0)
        return args
