import matplotlib.pyplot as plt

# 数据
data = [[146, 4, 20, 30],
 [54, 102, 25, 19],
 [68, 38, 41, 53],
 [77, 70, 36, 17],
 [140, 33, 11, 16]]
labels = ['type1', 'type2', 'type3', 'type4']
colors=["#d5695d", "#5d8ca8", "#65a479","#f0e442"]
# 创建一个2x1的子图布局
fig = plt.figure()
gs = fig.add_gridspec(2, 1)

# 在上面的子布局中绘制三个饼图
ax1 = fig.add_subplot(gs[0,:])
gs1 = gs[0,:].subgridspec(1,3)
#for i in range(3):
#    ax = fig.add_subplot(gs1[0,i])
#    ax.pie(data[i], labels=labels)
#    ax.axis("off")
ax = fig.add_subplot(gs1[0,0])
ax.pie(data[0], colors= colors)
ax.axis("off")
ax.text(-1,-1.3,'sx-mathoverflow')

ax = fig.add_subplot(gs1[0,1])
ax.pie(data[1],colors= colors )
ax.axis("off")
ax.text(-1.3,-1.3,'wiki-talk-temporal-100k')

ax = fig.add_subplot(gs1[0,2])
ax.pie(data[2],colors= colors )
ax.axis("off")
ax.text(-1.2,-1.3,'sx-stackoverflow-150k')
ax1.axis("off")
ax1.legend(labels, loc = 'upper center')
# 在下面的子布局中绘制两个饼图
ax2 = fig.add_subplot(gs[1,:])
gs2 = gs[1,:].subgridspec(1,2)
#for i in range(2):
#    ax = fig.add_subplot(gs2[0,i])
#    ax.pie(data[i+3], labels=labels)
#    ax.axis("off")

ax = fig.add_subplot(gs2[0,0])
ax.pie(data[0+3],colors= colors )
ax.axis("off")
ax.text(-1.1,-1.3,'sx-askubuntu-100k')

ax = fig.add_subplot(gs2[0,1])
ax.pie(data[1+3],colors= colors )
ax.axis("off")
ax.text(-0.6,-1.3,'CollegeMsg')

ax2.set_visible(False)
# 显示图形
fig.legend(labels, loc='upper left')
plt.show()
