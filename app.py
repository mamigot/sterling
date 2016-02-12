from functools import wraps
from flask import Flask, session, render_template, request, redirect, url_for
from storage import access

app = Flask(__name__)

def login_required(f):
    """Decorator that checks the validity of a session (and that it's there)"""
    @wraps(f)
    def decorated_function(*args, **kwargs):
        if g.user is None:
            return redirect(url_for('login', next=request.url))
        return f(*args, **kwargs)
    return decorated_function

@app.route('/register')
def register():
    pass

@app.route('/login')
def login():
    pass

@app.route('/')
@login_required
def index():
    pass

@app.route('/timeline')
@login_required
def timeline():
    pass

@app.route('/profile/<user_id>')
@login_required
def profile(user_id=None):
    pass

@app.route('/status/', methods=['POST', 'DELETE'])
@login_required
def status():
    pass


if __name__ == '__main__':
    app.run()
