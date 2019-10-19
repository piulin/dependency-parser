
from units import Sentence

class Set:

    def __init__ (self, file: str ):
        self.sentences_ = []

        filep = open(file, "r")
        sentence_stream = ""

        for line in filep:

            if line != '\n':
                sentence_stream += line
            else:
                self.sentences_.append ( Sentence ( sentence_stream [:-1] ) )
                sentence_stream = ""


    def write (self, file: str):

        filep = open ( file, "w+" )

        for sentence in self.sentences_:
            sentence.print( lambda stream : filep.write ( stream ) )


    def count_tokens ( self ):
        return sum ( stc.length() for stc in self.sentences_ )

    def count_sentences ( self ):
        return len ( self.sentences_ )











