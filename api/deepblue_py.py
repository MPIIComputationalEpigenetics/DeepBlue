import xmlrpclib

cmd_tmpl = """
  def %(name)s(self, %(parameter_names)s):
    return self.server.%(name)s(%(parameter_names)s)
"""

def main():

  #client = xmlrpclib.Server("http://deepblue.mpi-inf.mpg.de/xmlrpc", allow_none=True)
  client = xmlrpclib.Server("http://localhost:31415", allow_none=True)

  (s, v) = client.echo(None)

  version = v.split()[1][1:-1]

  ok, commands = client.commands()
  if not ok:
    print "unable to retrieve commands"
    return

  categories = {}
  commands_long_doc = ""

  for name in sorted(commands.keys()):
    cmd = commands[name]
    desc = cmd["description"]

    category = desc[0]

    html_id = name.replace(' ', '-').lower()

    # generate full description html
    params_s = ""
    param_names = []
    param_names_convertion = []

    params_documentation = []
    titles = []
    for p in cmd["parameters"]:

      if p[0] == "user_key":
        param_names.append("user_key=deepblue_USER_KEY")
      elif p[0] == "extra_metadata":
        param_names.append("extra_metadata=None")
      else:
        param_names.append(p[0]+"=None")

    if param_names_convertion:
      parameters_list_convertion = ", " + ', '.join(param_names_convertion)
    else:
      parameters_list_convertion = ""

    commands_long_doc += cmd_tmpl % {"parameters_full": params_s,
                           "parameter_names": ', '.join(param_names),
                           "parameter_convertion": parameters_list_convertion,
                           "name": name,
                           'url': "deepblue_URL"}

  print commands_long_doc

  template = ""
  with open("api_auto.template.py", 'r') as f:
    template = f.read()
    template = template.replace("{{ version }}", version)
    template = template.replace("{{ commands }}", commands_long_doc)

  with open("deepblue_access.py", 'w') as f:
    f.write(template)

  print template

if __name__ == "__main__":
  main()
