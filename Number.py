
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import matplotlib.animation as animation
plt.rcParams['figure.figsize'] = 15, 8


plt.style.use('dark_background')
fig = plt.figure()
#creating a subplot
ax1 = fig.add_subplot(1,1,1)
n_players = 3
columns = ["Player number", "Money", "Aquired stocks", "Want to buy", "Want to sell", "Price", "Number of stock"]

def animate(i):
    df_tmp = pd.read_csv('prot.txt', sep=' ', index_col=False, names=["Player number", "Money", "Aquired stocks", "Want to buy", "Want to sell", "Price", "Number of stock"])
    mon = df_tmp["Number of stock"]
    player = pd.read_csv('Player_requests.txt', sep=' ', index_col=False, names=columns)
    number = len(player)
    
    inds = df_tmp["Player number"]==1
    
    ax1.clear()
    ax1.plot(np.arange(len(mon[inds][:number])), mon[inds][:number])

    plt.xlabel('Date')
    plt.ylabel('Number of stock')
    plt.title('Number of stock/day')
    plt.xticks([])
    ax1.set_xlim(-1, 100)


ani = animation.FuncAnimation(fig, animate, interval=1000)
plt.show()