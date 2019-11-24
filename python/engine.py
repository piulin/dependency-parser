from set import Set
from parsers import arcstandard
from models import random as rd
from models import oracle as oc
import assessment
import stopwatch

path = "/home/pedro/Documentos/CL/SDP/lab/datasets/english/train/"
path_dev = "/home/pedro/Documentos/CL/SDP/lab/datasets/english/dev/"

def run ( ) :

    s = stopwatch.stopwatch()
    s.start("total")
    print( 'Testing stuff...' )

    s.start("Read file")
    sample = Set(path + "wsj_train.only-projective.conll06")
    s.stop()


    aparser = arcstandard.Parser (sample)
    aoracle = arcstandard.Oracle (sample)
    s.start("Get oracle transitions")
    omodel = oc.Model(aoracle.transitions())
    s.stop()

    s.start("Parse random model")
    predicted_random = aparser.parse( rd.Model() )
    s.stop()

    s.start("Parse oracle")
    predicted_oracle = aparser.parse(omodel)
    s.stop()

    s.start("Assessing")
    print(assessment.uas(sample, predicted_oracle))
    print(assessment.uas(sample, predicted_random))
    s.stop()
    s.stop()

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