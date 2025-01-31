---
title: Folder Index
layout: default
---

# Pages in This Folder

<ul>
{% for page in site.pages %}
  {% if page.identifier == 'foldername' and page.url != page.url %}
    <li><a href="{{ page.url | relative_url }}">{{ page.title }}</a></li>
  {% endif %}
{% endfor %}
</ul>
