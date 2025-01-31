# colorboosting

Try to improve the color contrast of the image

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_colorboosting](../assets/img_out/dog_colorboosting.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_colorboosting](../assets/img_out/female_colorboosting.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_colorboosting](../assets/img_out/male_colorboosting.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_colorboosting](../assets/img_out/tree_colorboosting.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.colorboosting
bypass=false

[info]
description=Try to improve the color contrast of the image
fileType=ibp.imagefilterlist
nFilters=1
name=Color Boosting


```
