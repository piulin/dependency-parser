from set import Set
from parsers import arcstandard
from models import random as rd
from models import oracle as oc

path = "/home/pedro/Documentos/CL/SDP/lab/datasets/english/train/"


def run ( ) :


    print( 'Testing stuff...' )

    sample = Set(path + "sample.conll06")
    stc = sample.sentences_[0]

    prs = arcstandard.Parser(stc)
    prs.parse( rd.Model ( ) )

    orc = arcstandard.Oracle ( stc )
    tr = orc.transitions ( )

    prs2 = arcstandard.Parser(stc)
    prs2.parse( oc.Model( tr ) )



    #gold = Set (path + "wsj_dev.conll06.gold")
    #pred = Set (path + "wsj_dev.conll06.pred")

    #print ( assessment.uas(gold, pred) )
    #print ( assessment.las(gold, pred) )

    #args = ps()

    #s = Set ( args.input_coll06_path  )
    #s.write ( args.output_coll06_path )


if __name__ == "__main__":
    run()