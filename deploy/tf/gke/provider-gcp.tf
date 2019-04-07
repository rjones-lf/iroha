provider "google" {
  credentials = "${file("k8s-ledger-e63faea4e2d5.json")}"
  project     = "k8s-ledger"
  region      = "us-west2"
}
