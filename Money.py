
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import matplotlib.animation as animation
plt.rcParams['figure.figsize'] = 15, 8


plt.style.use('dark_background')
fig = plt.figure()
#creating a subplot
ax1 = fig.add_subplot(1,1,1)
n_players = 6
columns = ["Player number", "Money", "Aquired stocks", "Want to buy", "Want to sell", "Price", "Number of stock"]
names = ["Программа-дурачок", "XGBoost", "Тривиальная оценка", "Фильтр Калмана", "Random Forest", "Игрок"]


def animate(i):
    df_tmp = pd.read_csv('prot.txt', sep=' ', index_col=False, names=columns)
    player = pd.read_csv('Player_requests.txt', sep=' ', index_col=False, names=columns)
    df_tmp = pd.concat((df_tmp, player))
    df_tmp = df_tmp[["Player number", "Money"]]
    kalman = pd.read_csv('Kalman.txt', sep=' ', index_col=False, names=["Money"])
    kalman["Player number"] = 4*np.ones(len(kalman), int)
    rf = pd.read_csv('RF.txt', sep=' ', index_col=False, names=["Money"])
    rf["Player number"] = 5*np.ones(len(rf), int)
    df_tmp = pd.concat((df_tmp, kalman, rf))
    mon = df_tmp["Money"]
    
    inds = [df_tmp["Player number"]==i for i in range(1, n_players+1)]
    number = len(player)
    ax1.clear()
    for i in range(n_players):
        ax1.plot(np.arange(len(mon[inds[i]][:number])), mon[inds[i]][:number], label=names[i])
    ax1.set_xlim(-1, 100)

    plt.xlabel('Date')
    plt.ylabel('Money')
    plt.title('Money/day')
    plt.xticks([])
    plt.legend()


ani = animation.FuncAnimation(fig, animate, interval=1000)
plt.show()