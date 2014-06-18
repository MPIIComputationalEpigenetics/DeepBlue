from client import EpidbClient

category_tmpl = """
<section id="api-%(id)s" class="panel">
  <header>
    <h2>%(description)s</h2>
  </header>
  <div class="panel-body">

  %(commands)s

  </div>
</section>
"""

cmd_tmpl = """
    <div class="api-endpoint">
      <h3 id="api-%(id)s">%(name)s</h3>

      <div class="api-description">%(description)s</div>

      <div class="api-header">
        <code class="operation-name">%(name)s</code><code class="operation-parameter">(%(parameter_names)s)</code>
      </div>

      <h4>Parameters:</h4>
      <ul class="api-parameters">
        %(parameters_full)s
      </ul>

      <h4>Responses:</h4>
      <ul class="api-responses">
        <li>
          <code class="python-code">['okay', result]</code>
          &mdash; result consists of
          <ul class="api-parameters">
            %(results_full)s
          </ul>
        </li>
        <li>
          <code class="python-code">['error', error_message]</code>
          &mdash; Error. Verify the error message.
        </li>
      </ul>
    </div>
"""

param_tmpl = """
        <li><code class="operation-parameter">%(param_name)s</code> (<code class="operation-type">%(param_type)s</code>) &mdash; %(param_description)s</li>
"""
param_multiple_tmpl = """
        <li><code class="operation-parameter">%(param_name)s</code> (<span class="optional">[</span><code class="operation-type">%(param_type)s</code><span class="optional">, ...]</span>) &mdash; %(param_description)s</li>
"""



category_short_tmpl = """
<li>
  <a href="#api-%(id)s">%(name)s</a>
  &mdash; %(description)s
  <ol>
%(commands)s
  </ol>
</li>
"""

cmd_short_tmpl = """
    <li><a href="#api-%(id)s">%(name)s</a> &mdash; %(description)s</li>
"""


def main():
  client = EpidbClient()

  (s, v) = client.echo(None)

  version = v.split()[2]

  ok, commands = client.commands()
  if not ok:
    print "unable to retrieve commands"
    return

  categories = {}

  for name in sorted(commands.keys()):
    cmd = commands[name]
    desc = cmd["description"]
    category = desc[0]

    html_id = name.replace(' ', '-').lower()

    if not category in categories:
      categories[category] = {"description": desc[1], "short": [], "long": []}

    # generate short description html
    short_doc = cmd_short_tmpl % {"id": html_id,
                      "name": name,
                      "description": desc[2][:desc[2].find('.')]}

    categories[category]["short"].append(short_doc)

    # generate full description html
    params_s = ""
    param_names = []
    for p in cmd["parameters"]:
      param_names.append(p[0])
      if p[2]:
        params_s += param_multiple_tmpl % {"param_name": p[0], "param_type": p[1], "param_description": p[3] }
      else:
        params_s += param_tmpl % {"param_name": p[0], "param_type": p[1], "param_description": p[3] }
      params_s += '\n'

    results_s = ""
    for r in cmd["results"]:
      results_s += param_tmpl % {"param_name": r[0], "param_type": r[1], "param_description": r[3] }
      results_s += '\n'

    long_doc = cmd_tmpl % {"parameters_full": params_s,
                           "results_full": results_s,
                           "parameter_names": ', '.join(param_names),
                           "name": name,
                           "description": desc[2],
                           "id": html_id}

    categories[category]["long"].append(long_doc)


  commands_long_doc = ""
  commands_short_doc = ""

  for category in sorted(categories.keys()):
    html_id = category.replace(' ', '-').lower()

    commands_long_doc += category_tmpl % {"id": html_id,
                                          "description": categories[category]["description"],
                                          "commands": '\n\n'.join(categories[category]["long"]) }

    commands_short_doc += category_short_tmpl % {"id": html_id,
                                                 "name": category,
                                                 "description": categories[category]["description"],
                                                 "commands": '\n\n'.join(categories[category]["short"]) }


  template = ""
  with open("api_auto.template", 'r') as f:
    template = f.read()
    template = template.replace("{{ version }}", version)
    template = template.replace("{{ commands }}", commands_long_doc)
    template = template.replace("{{ commands_short }}", commands_short_doc)

  with open("api_auto.html", 'w') as f:
    f.write(template)

  print "documentation written to api_auto.html"


if __name__ == "__main__":
  main()
