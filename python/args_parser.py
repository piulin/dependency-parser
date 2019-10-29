import argparse


def parse ():

    parser = argparse.ArgumentParser(description='Load and write coll06 files.')

    parser.add_argument('input_coll06_path', metavar='input_coll06_path', type=str,
                        help='The file to be read in coll06 format.')

    parser.add_argument('-o','--output', type=str, default='out.coll06', help="Write the file loaded into coll06 format.",
                        metavar='output_coll06_path',dest='output_coll06_path')

    return parser.parse_args()

