# guidedfilter

Smooth the image preserving hard edges

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_guidedfilter](../assets/img_out/dog_guidedfilter.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_guidedfilter](../assets/img_out/female_guidedfilter.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_guidedfilter](../assets/img_out/male_guidedfilter.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_guidedfilter](../assets/img_out/tree_guidedfilter.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.guidedfilter
bypass=false
edgepreservation=80
radius=40

[info]
description=Smooth the image preserving hard edges
fileType=ibp.imagefilterlist
nFilters=1
name=Guided Filter


```
