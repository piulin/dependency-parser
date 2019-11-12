from set import Set
from parsers import arcstandard
from models import random as rd
from models import oracle as oc
import assessment

path = "/home/pedro/Documentos/CL/SDP/lab/datasets/english/train/"
path_dev = "/home/pedro/Documentos/CL/SDP/lab/datasets/english/dev/"

def run ( ) :


    print( 'Testing stuff...' )

    gold = Set (file=path_dev + "wsj_dev.conll06.gold")
    pred = Set (file=path_dev + "wsj_dev.conll06.pred")

    #print ( assessment.uas(gold, pred) )
    #print ( assessment.las(gold, pred) )

    # sample = Set(path + "sample.conll06")
    sample = Set(path + "wsj_train.only-projective.conll06")
    # sample = Set(path + "sample2.conll06")

    aparser = arcstandard.Parser (sample)
    aoracle = arcstandard.Oracle (sample)
    omodel = oc.Model(aoracle.transitions())
    predicted_random = aparser.parse( rd.Model() )
    predicted_oracle = aparser.parse(omodel)

    print(assessment.uas(sample, predicted_oracle))
    print(assessment.uas(sample, predicted_random))

    # stc = sample.sentences_[0]
    #
    # prs = arcstandard.Parser.Stc_parser(stc)
    # sent_rd = prs.parse( rd.Model ( ) )
    #
    # orc = arcstandard.Oracle.Stc_oracle ( stc )
    # tr = orc.transitions ( )
    #
    # prs2 = arcstandard.Parser.Stc_parser(stc)
    # sent_tr = prs2.parse( oc.Model( tr ) )
    #
    #
    # set_tr = Set (sentences= [sent_tr] )
    # set_rd = Set (sentences= [sent_rd] )
    #
    # print ( assessment.uas(sample,set_tr))
    #
    # print ( assessment.uas(sample,set_rd))
    #
    # set_tr.write(file="tr.conll06")
    # set_rd.write(file="rd.conll06")




    #args = ps()

    #s = Set ( file=args.input_coll06_path  )
    #s.write ( args.output_coll06_path )


if __name__ == "__main__":
    run()