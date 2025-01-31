# prospectiveiihc

Removes the artifacts due to a bad illumination using a prospective approach

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_prospectiveiihc](../assets/img_out/dog_prospectiveiihc.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_prospectiveiihc](../assets/img_out/female_prospectiveiihc.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_prospectiveiihc](../assets/img_out/male_prospectiveiihc.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_prospectiveiihc](../assets/img_out/tree_prospectiveiihc.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.prospectiveiihc
bypass=false
image=3
outputmode=22

[info]
description=Removes the artifacts due to a bad illumination using a prospective approach
fileType=ibp.imagefilterlist
nFilters=1
name=Prospective IIH Correction


```
