---
title: Plugins
layout: default
---

# Plugins

<ul>
{% for page in site.pages %}
  {% assign path_parts = page.path | split: '/' %}
  {% if path_parts[0] == 'plugins' and page.path != 'plugins/index.md' and page.path contains '.md' %}
    <li><a href="{{ page.url | relative_url }}">{{ page.title }}</a></li>
  {% endif %}
{% endfor %}
</ul>
