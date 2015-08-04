import mail


def from_email(context):
    sentinel = 'terms on' # ends when this string is seen
    values = {}
    for line in iter(raw_input, sentinel):
        k, v = line.split(" ", 1)
        values[k] = v

    mandatory_keys = ["username", "email", "password", "passwordConfirm", "affiliation"]
    optional_fields = ["subscription"]

    for mk in mandatory_keys:
        if not values.has_key(mk):
            print "missing " + mk
            return

    if values["password"] != values["passwordConfirm"]:
        print "Passwords does not match"
        return

    user_name = values["username"]
    email = values["email"]
    affiliation = values["affiliation"]

    password = values["password"]

    epidb = context.epidb

    (s, u) = epidb.add_user(user_name, email, affiliation, context.user_key)
    if s == "error":
        print user_key

    _id, user_key = u
    (s, _id) = epidb.modify_user("password", password, user_key)
    if s == "error":
        print _id

    (s, _id) = epidb.modify_user_admin(_id, "permission_level", "GET_DATA", context.user_key)

    if s == 'okay':
        print('Okay.')
        print(_id, user_key)
        mail.send_new_user_mail(user_name, user_key, email)
    else:
        print(_id)


def create_user(context):
    okay = False
    while not okay:
        user_name = raw_input("User name: ")
        email = raw_input("Email: ")
        institution = raw_input("Institution: ")

        in_okay = raw_input('Are these informations correct? (Type Y for Yes)')

        if in_okay == 'Y':
            okay = True

    epidb = context.epidb
    (s, user_key) = epidb.add_user(user_name, email, institution,
                                       context.user_key)
    if s == 'okay':
        print('Okay.')
        print(user_key)
        mail.send_new_user_mail(user_name, user_key[1], email)
    else:
        print(user_key)


def list_users(context):
    epidb = context.epidb
    (status, users) = epidb.list_users(context.user_key)
    if status == "error":
        print status, users
        return
    for user in users:
        print user

commands_callback = {'e': from_email, 'create_user': create_user, 'list_users': list_users}


def get_command(name):
    if not name in commands_callback:
        print 'Invalid command ', name
        return None
    return commands_callback[name]
