---
title: 04 Plugins
layout: default
---

# Plugins

| Image | Image | 
|-----|--------|
| ![dog](../assets/img_in/dog.jpg) | ![female](../assets/img_in/female.jpg) |
| ![male](../assets/img_in/male.jpg) | ![tree](../assets/img_in/tree.jpg) |

{% for page in site.pages %}
{% assign path_parts = page.path | split: '/' %}{% if path_parts[0] == 'plugins' and page.path != 'plugins/index.md' and page.path contains '.md' %}
## [{{ page.title }}]({{ page.url | relative_url }})

| {{ page.title }} | {{ page.title }} | 
|-----|--------|
| ![dog_{{ page.title }}](../assets/img_out/dog_{{ page.title }}.jpg) | ![female_{{ page.title }}](../assets/img_out/female_{{ page.title }}.jpg) |
| ![male_{{ page.title }}](../assets/img_out/male_{{ page.title }}.jpg) | ![tree_{{ page.title }}](../assets/img_out/tree_{{ page.title }}.jpg) |

{% endif %}
{% endfor %}
