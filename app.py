from functools import wraps
from flask import Flask, g, session, render_template, request, redirect, url_for
import storage

app = Flask(__name__)

def login_required(f):
    @wraps(f)
    def decorated_function(*args, **kwargs):
        if session.get('username') is None:
            return redirect(url_for('login', next=request.url))
        return f(*args, **kwargs)
    return decorated_function

@app.route('/register', methods=['GET', 'POST'])
def register():
    if request.method == 'POST':
        pass
    else:
        return render_template('register.html')

@app.route('/login', methods=['GET', 'POST'])
def login():
    if request.method == 'POST':
        user = storage.User(request.form['username'], request.form['password'])

        if user.verify_credentials():
            session['username'] = user.username
            return redirect(url_for('timeline'))

    return render_template('login.html')

@app.route('/signout', methods=['POST'])
def signout():
    session.pop('username')
    return redirect(url_for('login'))

@app.route('/')
@app.route('/timeline')
@login_required
def timeline():
    print(session.get('username'))
    username = session.get('username')

    posts = []
    for i in range(10):
        posts.append(storage.Post(text='jajajajaja', username='joe', timestamp='123456790'))

    return render_template('timeline.html', username=username, posts=posts)

@app.route('/profile')
@app.route('/profile/<secondary_username>', methods=['GET', 'POST'])
@login_required
def profile(secondary_username=None):
    if request.method == 'GET':
        # Get the user's profile
        username = session.get('username')
        return render_template('profile.html', username=username, secondary_username=secondary_username)
    else:
        # Modify the user's relationship with somebody else (follow, unfollow, etc.)
        pass

@app.route('/post', methods=['GET', 'POST', 'PUT', 'DELETE'])
@login_required
def post():
    if request.method == 'GET':
        # Get
        return render_template('post.html')
    elif request.method == 'POST':
        # Save
        pass
    elif request.method == 'PUT':
        # Update
        pass
    else:
        # Delete
        pass

if __name__ == '__main__':
    app.secret_key = 'badsecretkey'
    app.debug = True

    app.run()
