from models import model
import random
from datetime import datetime

class Model (model.Model):

    def __init__(self):
        random.seed(datetime.now())

    def score_transitions(self, tk1, tk2):
        return [ random.random() for i in [1,2,3] ]