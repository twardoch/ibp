# domaintransformfilter

Smooth the image preserving hard edges

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_domaintransformfilter](../assets/img_out/dog_domaintransformfilter.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_domaintransformfilter](../assets/img_out/female_domaintransformfilter.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_domaintransformfilter](../assets/img_out/male_domaintransformfilter.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_domaintransformfilter](../assets/img_out/tree_domaintransformfilter.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.domaintransformfilter
bypass=false
edgepreservation=70
radius=40

[info]
description=Smooth the image preserving hard edges
fileType=ibp.imagefilterlist
nFilters=1
name=Domain Transform Filter


```
