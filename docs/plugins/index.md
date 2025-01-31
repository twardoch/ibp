---
title: 04 Plugins
layout: default
---

# Plugins

![female](../assets/img_in/female.jpg)

{% for page in site.pages %}
{% assign path_parts = page.path | split: '/' %}{% if path_parts[0] == 'plugins' and page.path != 'plugins/index.md' and page.path contains '.md' %}
## [{{ page.title }}]({{ page.url | relative_url }})

![female_{{ page.title }}](../assets/img_out/female_{{ page.title }}.jpg){% endif %}

{% endfor %}
