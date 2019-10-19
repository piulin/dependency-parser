from set import Set
from args_parser import parse as ps
import assessment

path = "/home/pedro/Documentos/CL/SDP/lab/english/dev/"


def run ( ) :


    print( 'Testing stuff...' )

    gold = Set (path + "wsj_dev.conll06.gold")
    pred = Set (path + "wsj_dev.conll06.pred")

    print ( assessment.uas(gold, pred) )
    print ( assessment.las(gold, pred) )

    args = ps()

    s = Set ( args.input_coll06_path  )
    s.write ( args.output_coll06_path )


if __name__ == "__main__":
    run()