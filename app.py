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
        user = storage.User(request.form['username'], request.form['password'])
        user.save_credentials()

        # proceed once we know the credentials are stored
        if user.verify_credentials():
            session['username'] = user.username
            return redirect(url_for('timeline'))

    return render_template('register.html')

@app.route('/login', methods=['GET', 'POST'])
def login():
    if request.method == 'POST':
        user = storage.User(request.form['username'], request.form['password'])

        if user.verify_credentials():
            session['username'] = user.username
            return redirect(url_for('timeline'))

    return render_template('login.html')

@app.route('/signout', methods=['GET', 'POST'])
@login_required
def signout():
    session.pop('username')
    return redirect(url_for('login'))

@app.route('/')
@app.route('/timeline', methods=['GET'])
@login_required
def timeline():
    user = storage.User(username=session.get('username'))
    posts = user.get_timeline_posts()
    return render_template('timeline.html', username=user.username, posts=posts)

@app.route('/profile')
@app.route('/profile/<secondary_username>', methods=['GET'])
@login_required
def profile(secondary_username=None):
    user = storage.User(username=session.get('username'))

    if secondary_username:
        # Fetch the secondary user's posts and the relationship to the main user
        secondary_user = storage.User(username=secondary_username)
        posts = secondary_user.get_profile_posts()

        is_following = user.is_following(secondary_user)
        is_followed_by = secondary_user.is_following(user)

        return render_template('profile.html', username=user.username, posts=posts,
            secondary_username=secondary_username, is_following=is_following,
            is_followed_by=is_followed_by)

    else:
        # Fetch our own profile
        posts = user.get_profile_posts()
        return render_template('profile.html', username=user.username, posts=posts)

@app.route('/followers', methods=['GET'])
@login_required
def followers():
    user = storage.User(username=session.get('username'))
    follower_usernames = [f.username for f in user.get_followers()]

    return render_template('followers.html', username=user.username,
        followers=follower_usernames)

@app.route('/friends', methods=['GET', 'POST'])
@login_required
def friends():
    user = storage.User(username=session.get('username'))

    if request.method == 'GET':
        friend_usernames = [f.username for f in user.get_friends()]

        return render_template('friends.html', username=user.username,
            friends=friend_usernames)

    elif request.method == 'POST':
        secondary_user = storage.User(username=request.form.get('secondary_username'))

        if request.form.get('follow'):
            user.follow(secondary_user)
        elif request.form.get('unfollow'):
            user.unfollow(secondary_user)

        return redirect(url_for('friends'))

@app.route('/post', methods=['POST'])
@login_required
def post():
    user = storage.User(username=session.get('username'))

    if request.form.get('savepost'):
        post = storage.Post(text=request.form.get('text'))
        user.save_post(post)

    elif request.form.get('deletepost'):
        user.delete_post(request.form.get('timestamp'))

    return redirect(url_for('profile'))

if __name__ == '__main__':
    app.secret_key = 'badsecretkey'
    app.debug = True

    app.run()
