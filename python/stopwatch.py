import time


class stopwatch:

    def __init__(self):

        self.instantiated_ = time.time()
        self.mes_stack_ = []
        self.time_stack_ = []

    def start (self, mes):
        t = time.time()
        self.mes_stack_.append( mes )
        self.time_stack_.append( t )
        print(f"[TIMER STARTED AT {t - self.instantiated_:.2f}]: '{mes}'")


    def stop (self, mes=""):
        ts = time.time()
        rf = ts
        if ( mes == "" ):
            mes = self.mes_stack_.pop()
            rf = self.time_stack_.pop()
        else :
            i = self.mes_stack_.index( mes )
            rf = self.time_stack_.pop(i)
            self.mes_stack_.pop(i)


        print( f"---[TIMER STOPPED AT {ts - self.instantiated_:.2f}]: '{mes}' took {ts - rf:.4f} s." )







