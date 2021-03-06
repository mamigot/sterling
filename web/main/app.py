from flask import Flask


app = Flask(__name__)
app.secret_key = 'badsecretkey'
app.debug = True


######## Routes ########
from routes import *

######## Register filters ########
from filters import filters

for name, func in filters.items():
    app.jinja_env.filters[name] = func


if __name__ == '__main__':
    app.run()
