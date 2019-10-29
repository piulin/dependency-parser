from parsers.dep_parser import State
import units
import copy
import models.model


class Parser:

    def __init__(self , stc : units.Sentence ):
        self.root = stc.tokens_ [ 0 ]
        arcs = dict.fromkeys( stc.tokens_ , ( ) )
        self.states_ = [ State (tuple ( stc.tokens_ [1:] ) , tuple( [ self.root ] ) , arcs ) ]


    def curr_state(self):
        return self.states_ [ -1 ]

    def terminal_state (self):
        return True if ( self.curr_state().stack_ and not self.curr_state().buff_ ) or \
                       (not self.curr_state().stack_ and not self.curr_state().buff_ ) else False

    def score_transitions(self, model : models.model):
        # Pop the stack, look the first element in buffer
        # Ask the model, blah blah blah
        #
        # 0: shift, 1: larc, 2: rarc
        return model.score_transitions (0,0) # to be defined in the future

    def perform_first_valid( self, scores ):

        while len(scores) > 0:
            mi = scores.index ( max ( scores ) )
            if self.can_switcher [ mi ] ( self ) :
                self.do_switcher [ mi ] ( self )
                return
            scores [ mi ] = -1.0

        raise RuntimeError ( 'None of the transitions were eligible.' )


    def parse (self, model : models.model ):
        print ('Initial state:')
        print(self.curr_state())
        while not self.terminal_state ( ):
            scores = self.score_transitions ( model )
            self.perform_first_valid(scores)
            print(self.curr_state())

    def can_leftarc(self):
        try:
            return True if self.curr_state().stack_ [ -1 ] != self.root else False
        except:
            return False

    def can_rightarc(self):
        return True if self.curr_state().buff_ and self.curr_state().stack_ else False

    def can_shift (self):
        return True if len ( self.curr_state().buff_ ) > 1 or not self.curr_state().stack_ else False

    def shift (self):
        print ("Shifting...")
        curr_state = self.curr_state()
        self.states_.append( State ( curr_state.buff_ [1:],
                                     curr_state.stack_ + (curr_state.buff_ [0],),
                                     copy.copy( curr_state.arcs_ ) ) )
    def leftarc(self):

        print ("Left arc...")
        curr_state = self.curr_state()
        narcs = copy.copy( curr_state.arcs_ )
        narcs [ curr_state.buff_ [0] ] += ( curr_state.stack_ [ -1 ], )
        nstack = curr_state.stack_ [ :-1 ]
        self.states_.append( State( curr_state.buff_ , nstack , narcs ) )

    def rightarc(self):

        print ("Right arc...")
        curr_state = self.curr_state()
        narcs = copy.copy( curr_state.arcs_ )
        narcs [ curr_state.stack_ [ -1 ]  ] += ( curr_state.buff_ [0], )
        nstack = curr_state.stack_ [ :-1 ]

        if curr_state.stack_ [ -1 ] == self.root:
            nbuff = curr_state.buff_ [ 1: ]
        else:
            nbuff = ( curr_state.stack_ [ -1 ], ) + curr_state.buff_ [ 1: ]


        self.states_.append( State( nbuff , nstack , narcs ) )

    do_switcher = {
        0:shift,
        1:leftarc,
        2:rightarc
    }
    can_switcher = {
        0: can_shift,
        1: can_leftarc,
        2: can_rightarc
    }

class Oracle:

    def __init__(self, stc: units.Sentence):

        self.parser_ = Parser( stc )
        self.trans_ = []
        #initialize the gold arc
        self.gold_arcs =  { tk:[] for tk in stc.tokens_ }
        #avoid the root
        for tk in stc.tokens_ [1:]:
            head = tk.head_
            self.gold_arcs [ stc.tokens_ [ head ] ].append( tk )

    def transitions (self):

        print('Getting transitions...')
        while not self.terminal_state():
            #print ( self.parser_.curr_state() )
            if self.should_leftarc():
                self.parser_.leftarc()
                self.trans_.append(Parser.leftarc)

            elif self.should_rightarc():
                self.parser_.rightarc()
                self.trans_.append(Parser.rightarc)

            else:
                self.parser_.shift()
                self.trans_.append(Parser.shift)

        return self.trans_

    def terminal_state (self):
        return True if not self.parser_.curr_state().buff_ else False

    def should_leftarc(self):
        state = self.parser_.curr_state()
        return True if self.parser_.can_leftarc() and state.stack_ [ -1 ] in self.gold_arcs [ state.buff_ [0] ] else False

    def should_rightarc(self):
        state = self.parser_.curr_state()
        return True if self.parser_.can_rightarc() and (state.buff_[ 0 ] in self.gold_arcs[ state.stack_[ -1 ] ] ) \
                      and self.has_all_children( state.buff_[ 0 ] ) else False

    def has_all_children (self, tk):
        return True if set ( self.gold_arcs [ tk ] ) == set ( self.parser_.curr_state().arcs_ [ tk ] ) else False
