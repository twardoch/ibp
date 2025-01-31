# adaptivemanifoldfilter

Smooth the image preserving hard edges

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_adaptivemanifoldfilter](../assets/img_out/dog_adaptivemanifoldfilter.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_adaptivemanifoldfilter](../assets/img_out/female_adaptivemanifoldfilter.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_adaptivemanifoldfilter](../assets/img_out/male_adaptivemanifoldfilter.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_adaptivemanifoldfilter](../assets/img_out/tree_adaptivemanifoldfilter.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.adaptivemanifoldfilter
bypass=false
edgepreservation=75
radius=7500.0

[info]
description=Smooth the image preserving hard edges
fileType=ibp.imagefilterlist
nFilters=1
name=Adaptive Manifold Filter


```
