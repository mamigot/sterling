from functools import wraps
from flask import session, render_template, request, redirect, url_for, flash
from access import User, ErrorProcessingRequest
from app import app


def login_required(f):
    """Gateway for all functions that require the user to be logged in.
    If the user's username is stored in the session dict, the provided
    function is called with its arguments. Otherwise, the user is redirected
    to log in.
    """
    @wraps(f)
    def decorated_function(*args, **kwargs):
        if session.get('username') is None:
            return redirect(url_for('login'))
        return f(*args, **kwargs)
    return decorated_function

@app.route('/register', methods=['GET', 'POST'])
def register():
    if session.get('username'):
        return render_template('404.html',
            message='Cannot register while you\'re logged in. (╯°□°）╯︵ ┻━┻'
        )

    elif request.method == 'POST':
        user = User(request.form['username'], request.form['password'])

        try:
            # Proceed once we know the credential is stored
            user.save_credential()
            session['username'] = user.username
            return redirect(url_for('timeline'))
        except ErrorProcessingRequest:
            flash('Failed registration –try a different username')

    return render_template('register.html')

@app.route('/deactivate', methods=['GET', 'POST'])
def deactivate():
    if session.get('username'):
        return render_template('404.html',
            message='Cannot deactivate while you\'re logged in. (╯°□°）╯︵ ┻━┻'
        )

    elif request.method == 'POST':
        user = User(request.form['username'], request.form['password'])

        try:
            user.delete_credential()
            flash('Successful deactivation')
        except ErrorProcessingRequest:
            flash('Failed deactivation')

    return render_template('deactivate.html')

@app.route('/login', methods=['GET', 'POST'])
def login():
    if session.get('username'):
        return render_template('404.html',
            message='Cannot log in while you\'re logged in. (╯°□°）╯︵ ┻━┻'
        )

    elif request.method == 'POST':
        user = User(request.form['username'], request.form['password'])

        try:
            user.verify_credential()
            session['username'] = user.username
            return redirect(url_for('timeline'))
        except ErrorProcessingRequest:
            flash('Failed login –cannot verify the credential')

    return render_template('login.html')

@app.route('/signout', methods=['GET', 'POST'])
@login_required
def sign_out():
    session.pop('username')
    flash('Successful signout')
    return redirect(url_for('login'))

@app.route('/')
@app.route('/timeline', methods=['GET'])
@login_required
def timeline():
    # Fetch the user from the system and render his timeline posts
    user = User(username=session.get('username'))

    return render_template('timeline.html',
        username=user.username,
        posts=user.get_timeline_posts()
    )

@app.route('/profile/')
@app.route('/profile/<requested_username>', methods=['GET'])
@login_required
def profile(requested_username=None):
    """If `secondary_username` corresponds to an existing user, show his page.
    If `secondary_username` does not exist, show a 404.
    If `secondary_username` is the user's own profile, redirect to his page.
    """
    user = User(session.get('username'))
    if not requested_username:
        requested_username = request.args.get('requested_username')

    if requested_username != session.get('username'):
        # Dealing with a secondary user (i.e. not the one who's logged in)
        secondary_user = User(requested_username)

        if not secondary_user.exists():
            return render_template('404.html',
                message='User "%s" does not exist' % secondary_user.username
            )

        # Fetch the secondary user's posts and the relationship to the main user
        return render_template('profile.html',
            username=user.username,
            posts=secondary_user.get_profile_posts(),
            secondary_username=secondary_user.username,
            is_following=user.is_following(secondary_user),
            is_followed_by=secondary_user.is_following(user)
        )

    else:
        # Fetch our own profile
        return render_template('profile.html',
            username=user.username,
            posts=user.get_profile_posts()
        )

@app.route('/followers', methods=['GET'])
@login_required
def followers():
    # Fetch the user from the system and render his followers
    user = User(username=session.get('username'))

    return render_template('followers.html',
        username=user.username,
        followers=[u.username for u in user.get_followers()]
    )

@app.route('/friends', methods=['GET', 'POST'])
@login_required
def friends():
    # Fetch the user from the system and render his friends
    user = User(username=session.get('username'))

    if request.method == 'GET':
        return render_template('friends.html',
            username=user.username,
            friends=[u.username for u in user.get_friends()]
        )

    elif request.method == 'POST':
        # Follow or unfollow the user, as specified
        secondary_user = User(username=request.form.get('secondary_username'))

        if request.form.get('follow'):
            user.follow(secondary_user)

        elif request.form.get('unfollow'):
            user.unfollow(secondary_user)

        return redirect(url_for('friends'))

@app.route('/post', methods=['POST'])
@login_required
def post():
    # Save or modify a post, as specified
    user = User(username=session.get('username'))

    if request.form.get('savepost'):
        user.save_post(text=request.form.get('text'))

    elif request.form.get('deletepost'):
        user.delete_post(request.form.get('timestamp'))

    return redirect(url_for('profile', requested_username=user.username))
