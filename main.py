from PyQt5 import QtWidgets, QtCore

class MainWindow(QtWidgets.QWidget):
    def __init__(self):
        super().__init__()
        self.show()


if __name__ == "__main__":
    app = QtWidgets.QApplication([])
    win = MainWindow()
    app.exec()